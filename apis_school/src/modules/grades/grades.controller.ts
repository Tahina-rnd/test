import {
  Controller,
  Get,
  Post,
  Body,
  Patch,
  Param,
  Delete,
  UseGuards,
} from '@nestjs/common';
import { GradesService } from './grades.service';
import { CreateGradeDto } from './dto/create-grade.dto';
import { UpdateGradeDto } from './dto/update-grade.dto';
import { Semester } from './entities/grade.entity';
import { JwtAuthGuard } from '@common/guards/jwt-auth.guard';
import { RolesGuard } from '@common/guards/roles.guard';
import { Roles } from '@decorators/roles.decorator';
import { UserRole } from '@enums/user.enum';

@Controller('grades')
@UseGuards(JwtAuthGuard, RolesGuard)
export class GradesController {
  constructor(private readonly gradesService: GradesService) {}

  /**
   * Create a new grade.
   *
   * @param createGradeDto - The grade data to create.
   *
   * @returns The created grade.
   */
  @Post()
  @Roles(UserRole.ADMIN)
  create(@Body() createGradeDto: CreateGradeDto) {
    return this.gradesService.create(createGradeDto);
  }

  /**
   * Get all grades.
   *
   * @returns An array of grades.
   */
  @Get()
  @Roles(UserRole.ADMIN)
  findAll() {
    return this.gradesService.findAll();
  }

  /**
   * Get a grade by id.
   *
   * @param id - The id of the grade to retrieve.
   *
   * @returns The grade.
   */
  @Get(':id')
  @Roles(UserRole.ADMIN, UserRole.STUDENT)
  findOne(@Param('id') id: string) {
    return this.gradesService.findOne(id);
  }

  /**
   * Get grades by student id.
   *
   * @param studentId - The student id to get grades for.
   *
   * @returns An array of grades.
   */
  @Get('student/:studentId')
  @Roles(UserRole.ADMIN, UserRole.STUDENT)
  findByStudentId(@Param('studentId') studentId: string) {
    return this.gradesService.findByStudentId(studentId);
  }

  /**
   * Get grades by subject code.
   *
   * @param subjectCode - The subject code to get grades for.
   *
   * @returns An array of grades.
   */
  @Get('subject/:subjectCode')
  @Roles(UserRole.ADMIN)
  findBySubject(@Param('subjectCode') subjectCode: string) {
    return this.gradesService.findBySubject(subjectCode);
  }

  /**
   * Get grades by academic period.
   *
   * @param academicYear - The academic year.
   * @param semester - The semester.
   *
   * @returns An array of grades.
   */
  @Get('academic/:academicYear/:semester')
  @Roles(UserRole.ADMIN)
  findByAcademicPeriod(
    @Param('academicYear') academicYear: string,
    @Param('semester') semester: Semester,
  ) {
    return this.gradesService.findByAcademicPeriod(academicYear, semester);
  }

  /**
   * Calculate GPA for a student.
   *
   * @param studentId - The student id to calculate GPA for.
   *
   * @returns The GPA.
   */
  @Get('gpa/:studentId')
  @Roles(UserRole.ADMIN, UserRole.STUDENT)
  calculateGPA(@Param('studentId') studentId: string) {
    return this.gradesService.calculateGPA(studentId);
  }

  /**
   * Update a grade.
   *
   * @param id - The id of the grade to update.
   * @param updateGradeDto - The grade data to update.
   *
   * @returns The updated grade.
   */
  @Patch(':id')
  @Roles(UserRole.ADMIN)
  update(@Param('id') id: string, @Body() updateGradeDto: UpdateGradeDto) {
    return this.gradesService.update(id, updateGradeDto);
  }

  /**
   * Delete a grade.
   *
   * @param id - The id of the grade to delete.
   */
  @Delete(':id')
  @Roles(UserRole.ADMIN)
  remove(@Param('id') id: string) {
    return this.gradesService.remove(id);
  }
}
